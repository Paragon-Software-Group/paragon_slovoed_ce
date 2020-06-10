/*
 * license_manager_api
 *
 *  Created on: 09.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api.test;

import com.paragon_software.license_manager_api.UserAccount;
import org.junit.Test;

import static org.junit.Assert.*;

public class UserAccountTest
{
  @Test
  public void createUserAccountByName() throws Exception
  {
    final String name = "User1";
    final UserAccount userAccount = UserAccount.userAccountByName(name);
    assertEquals("Name not same", name, userAccount.getName());
    assertNull("Id not empty", userAccount.getId());
    assertNull("EMail not empty", userAccount.getEMail());
    assertNull("Password not empty", userAccount.getPassword());
  }

  @Test
  public void createUserAccountByNameAndPassword() throws Exception
  {
    final String name = "User1";
    final String password = "P@ssw0rd";
    final UserAccount userAccount = UserAccount.userAccountByName(name, password);
    assertEquals("Name not same", name, userAccount.getName());
    assertNull("Id not empty", userAccount.getId());
    assertNull("EMail not empty", userAccount.getEMail());
    assertEquals("Password not same", password, userAccount.getPassword());
  }

  @Test
  public void createUserAccountByEMail() throws Exception
  {
    final String eMail = "User1@corp.com";
    final UserAccount userAccount = UserAccount.userAccountByEMail(eMail);
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertNull("Id not empty", userAccount.getId());
    assertNull("Name not empty", userAccount.getName());
    assertNull("Password not empty", userAccount.getPassword());
  }

  @Test
  public void createUserAccountEMailAndPassword() throws Exception
  {
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";
    final UserAccount userAccount = UserAccount.userAccountByEMail(eMail, password);
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertNull("Id not empty", userAccount.getId());
    assertNull("Name not empty", userAccount.getName());
    assertEquals("Password not same", password, userAccount.getPassword());
  }

  @Test
  public void createUserAccountByNameAndEMail() throws Exception
  {
    final String name = "User1";
    final String eMail = "User1@corp.com";
    final UserAccount userAccount = UserAccount.userAccountByNameAndEMail(name, eMail);
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertNull("Id not empty", userAccount.getId());
    assertEquals("Name not same", name, userAccount.getName());
    assertNull("Password not empty", userAccount.getPassword());
  }

  @Test
  public void createUserAccountNameAndEMailAndPassword() throws Exception
  {
    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";
    final UserAccount userAccount = UserAccount.userAccountByNameAndEMail(name, eMail, password);
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertNull("Id not empty", userAccount.getId());
    assertEquals("Name not same", name, userAccount.getName());
    assertEquals("Password not same", password, userAccount.getPassword());
  }

  @Test
  public void attachIdToAccountWithName() throws Exception
  {
    final String name = "User1";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String source = "BO4";
    final UserAccount userAccount = new UserAccount(UserAccount.userAccountByName(name), id, source);
    assertEquals("Id not same", id, userAccount.getId());
    assertEquals("Source not same", source, userAccount.getAccountSource());
    assertEquals("Name not same", name, userAccount.getName());
    assertNull("EMail not empty", userAccount.getEMail());
    assertNull("Password not empty", userAccount.getPassword());
  }

  @Test
  public void attachIdToAccountWithNameAndPassword() throws Exception
  {
    final String name = "User1";
    final String password = "P@ssw0rd";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String source = "BO4";
    final UserAccount userAccount = new UserAccount(UserAccount.userAccountByName(name, password), id, source);
    assertEquals("Id not same", id, userAccount.getId());
    assertEquals("Source not same", source, userAccount.getAccountSource());
    assertEquals("Name not same", name, userAccount.getName());
    assertNull("EMail not empty", userAccount.getEMail());
    assertEquals("Password not same", password, userAccount.getPassword());
  }

  @Test
  public void attachIdToAccountWithEMail() throws Exception
  {
    final String eMail = "User1@corp.com";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String source = "BO4";
    final UserAccount userAccount = new UserAccount(UserAccount.userAccountByEMail(eMail), id, source);
    assertEquals("Id not same", id, userAccount.getId());
    assertEquals("Source not same", source, userAccount.getAccountSource());
    assertNull("Name not empty", userAccount.getName());
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertNull("Password not empty", userAccount.getPassword());
  }

  @Test
  public void attachIdToAccountWithEMailAndPassword() throws Exception
  {
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String source = "BO4";
    final UserAccount userAccount = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id, source);
    assertEquals("Id not same", id, userAccount.getId());
    assertEquals("Source not same", source, userAccount.getAccountSource());
    assertNull("Name not empty", userAccount.getName());
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertEquals("Password not same", password, userAccount.getPassword());
  }

  @Test
  public void attachIdToAccountWithNameAndEMail() throws Exception
  {
    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String source = "BO4";
    final UserAccount userAccount = new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id, source);
    assertEquals("Id not same", id, userAccount.getId());
    assertEquals("Source not same", source, userAccount.getAccountSource());
    assertEquals("Name not same", name, userAccount.getName());
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertNull("Password not empty", userAccount.getPassword());
  }

  @Test
  public void attachIdToAccountWithNameAndEMailAndPassword() throws Exception
  {
    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String source = "BO4";
    final UserAccount userAccount =
        new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id, source);
    assertEquals("Id not same", id, userAccount.getId());
    assertEquals("Source not same", source, userAccount.getAccountSource());
    assertEquals("Name not same", name, userAccount.getName());
    assertEquals("EMail not same", eMail, userAccount.getEMail());
    assertEquals("Password not same", password, userAccount.getPassword());
  }

  @Test
  public void checkEqualsWithSameId() throws Exception
  {
    final String source = "BO4";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";

    final String name1 = "User1";
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";

    final String name2 = "User2";
    final String eMail2 = "User2@corp.com";
    final String password2 = "P@ssw0rd2";
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name2), id, source);
      assertEquals("Account not same", userAccount1, userAccount2);
      assertEquals("Account not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name1, password1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name2, password2), id, source);
      assertEquals("Account not same", userAccount1, userAccount2);
      assertEquals("Account not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail2), id, source);
      assertEquals("Account not same", userAccount1, userAccount2);
      assertEquals("Account not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail1, password1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail2, password2), id, source);
      assertEquals("Account not same", userAccount1, userAccount2);
      assertEquals("Account not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name1, eMail1), id, source);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name2, eMail2), id, source);
      assertEquals("Account not same", userAccount1, userAccount2);
      assertEquals("Account not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name1, eMail1, password1), id, source);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name2, eMail2, password2), id, source);
      assertEquals("Account not same", userAccount1, userAccount2);
      assertEquals("Account not same", userAccount2, userAccount1);
    }
  }

  @Test
  public void checkHashOnEqualsWithSameId() throws Exception
  {
    final String source = "BO4";
    final String id = "A3DB394E-AF60-4233-861B-7959549E9D4F";

    final String name1 = "User1";
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";

    final String name2 = "User2";
    final String eMail2 = "User2@corp.com";
    final String password2 = "P@ssw0rd2";
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name2), id, source);
      assertEquals("Hash for account not same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name1, password1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name2, password2), id, source);
      assertEquals("Hash for account not same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail2), id, source);
      assertEquals("Hash for account not same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail1, password1), id, source);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail2, password2), id, source);
      assertEquals("Hash for account not same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name1, eMail1), id, source);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name2, eMail2), id, source);
      assertEquals("Hash for account not same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name1, eMail1, password1), id, source);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name2, eMail2, password2), id, source);
      assertEquals("Hash for account not same", userAccount1.hashCode(), userAccount2.hashCode());
    }
  }

  @Test
  public void checkNotEqualsWithDifferentId() throws Exception
  {
    final String source1 = "BO4";
    final String source2 = "Facebook";
    final String id1 = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String id2 = "28486F1B-4A45-4EFD-83AB-BA3B52568268";

    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";

    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name), id2, source1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name), id1, source2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name, password), id2, source1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name, password), id1, source2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail), id2, source1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail), id1, source2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id2, source1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id1, source2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id2, source1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id1, source2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id2, source1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id1, source2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
  }

  @Test
  public void checkHashOnNotEqualsWithDifferentId() throws Exception
  {
    final String source1 = "BO4";
    final String source2 = "Facebook";
    final String id1 = "A3DB394E-AF60-4233-861B-7959549E9D4F";
    final String id2 = "28486F1B-4A45-4EFD-83AB-BA3B52568268";

    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";

    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name), id2, source1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name), id1, source2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name, password), id2, source1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByName(name, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByName(name, password), id1, source2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail), id2, source1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail), id1, source2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id2, source1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id1, source1);
      final UserAccount userAccount2 = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id1, source2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id2, source1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id1, source2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id2, source1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
    {
      final UserAccount userAccount1 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id1, source1);
      final UserAccount userAccount2 =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id1, source2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
  }

  @Test
  public void checkEqualsWithSameName() throws Exception
  {
    final String name1 = "User1";
    final String password1 = "P@ssw0rd";

    final String name2 = "User2";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name1);
      assertEquals("Account is same", userAccount1, userAccount2);
      assertEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name1, password1);
      assertEquals("Account is same", userAccount1, userAccount2);
      assertEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name1, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name2, password1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name2, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
  }

  @Test
  public void checkHashWithSameName() throws Exception
  {
    final String name1 = "User1";
    final String password1 = "P@ssw0rd";

    final String name2 = "User2";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name1);
      assertEquals("Hash for account is not same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name1, password1);
      assertEquals("Hash for account is not same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name1, password2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name2, password1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByName(name2, password2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
  }

  @Test
  public void checkEqualsWithSameEMail() throws Exception
  {
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";

    final String eMail2 = "User2@corp.com";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1);
      assertEquals("Account is same", userAccount1, userAccount2);
      assertEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password1);
      assertEquals("Account is same", userAccount1, userAccount2);
      assertEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail2, password1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail2, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
  }

  @Test
  public void checkHashWithSameEMail() throws Exception
  {
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";

    final String eMail2 = "User2@corp.com";
    final String password2 = "P@ssw0rd2";
    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1);
      assertEquals("Hash for account is not same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password1);
      assertEquals("Hash for account is not same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail2, password1);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }

    {
      final UserAccount userAccount1 = UserAccount.userAccountByEMail(eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail2, password2);
      assertNotEquals("Hash for account is same", userAccount1.hashCode(), userAccount2.hashCode());
    }
  }

  @Test
  public void checkEqualsWithNameAndEMail() throws Exception
  {
    final String name1 = "User1";
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";

    final String name2 = "User2";
    final String eMail2 = "User2@corp.com";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      assertEquals("Account is not same", userAccount1, userAccount2);
      assertEquals("Account is not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      assertEquals("Account is not same", userAccount1, userAccount2);
      assertEquals("Account is not same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail1, password1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail2, password1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail2, password1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail2, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
  }

  @Test
  public void checkHashWithNameAndEMail() throws Exception
  {
    final String name1 = "User1";
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";

    final String name2 = "User2";
    final String eMail2 = "User2@corp.com";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      assertEquals("Hash is not same", userAccount1.hashCode(), userAccount2.hashCode());
      assertEquals("Hash is not same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail2);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail1);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail2);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      assertEquals("Hash is not same", userAccount1.hashCode(), userAccount2.hashCode());
      assertEquals("Hash is not same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password2);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail1, password1);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name1, eMail2, password1);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail2, password1);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByNameAndEMail(name1, eMail1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByNameAndEMail(name2, eMail2, password2);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
  }

  @Test
  public void checkEqualsWithNameOrEMail() throws Exception
  {
    final String name1 = "User1";
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password1);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password2);
      assertNotEquals("Account is same", userAccount1, userAccount2);
      assertNotEquals("Account is same", userAccount2, userAccount1);
    }
  }

  @Test
  public void checkHashWithNameOrEMail() throws Exception
  {
    final String name1 = "User1";
    final String eMail1 = "User1@corp.com";
    final String password1 = "P@ssw0rd";
    final String password2 = "P@ssw0rd2";

    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password1);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
    {
      final UserAccount userAccount1 = UserAccount.userAccountByName(name1, password1);
      final UserAccount userAccount2 = UserAccount.userAccountByEMail(eMail1, password2);
      assertNotEquals("Hash is same", userAccount1.hashCode(), userAccount2.hashCode());
      assertNotEquals("Hash is same", userAccount2.hashCode(), userAccount1.hashCode());
    }
  }

  @Test
  public void checkEqualsWithSameNullAndOther() throws Exception
  {
    final UserAccount userAccount1 = UserAccount.userAccountByName("User1");
    assertEquals("Account is not same", userAccount1, userAccount1);
    assertNotEquals("Account is same", userAccount1, null);
    assertNotEquals("Account is same", userAccount1, "123");
  }

  @Test
  public void cloneObject() throws Exception
  {
    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";
    //noinspection SpellCheckingInspection
    final String id = "D9929D4A-56BF-4D03-99F3-0BBDFBC136B8";
    final String source = "Bo4";
    final UserAccount userAccount1 =
        new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id, source);
    final UserAccount userAccount2 = (UserAccount) userAccount1.clone();
    assertNotSame("Object is same", userAccount1, userAccount2);
    assertEquals("Account is not same", userAccount1, userAccount2);
    assertEquals("Id is not same", userAccount1.getId(), userAccount2.getId());
    assertEquals("Account source is not same", userAccount1.getAccountSource(), userAccount2.getAccountSource());
    assertEquals("Name is not same", userAccount1.getName(), userAccount2.getName());
    assertEquals("EMail is not same", userAccount1.getEMail(), userAccount2.getEMail());
    assertEquals("Password is not same", userAccount1.getPassword(), userAccount2.getPassword());
  }

  @Test
  public void toStringOperation() throws Exception
  {
    final String name = "User1";
    final String eMail = "User1@corp.com";
    final String password = "P@ssw0rd";
    //noinspection SpellCheckingInspection
    final String id = "D9929D4A-56BF-4D03-99F3-0BBDFBC136B8";
    final String source = "Bo4";
    {
      final UserAccount userAccount = UserAccount.userAccountByName(name);
      assertEquals("Invalid toString result", "{name='" + name + "'}", userAccount.toString());
    }
    {
      final UserAccount userAccount = UserAccount.userAccountByName(name, password);
      assertEquals("Invalid toString result", "{name='" + name + "', " + "password='" + password + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = UserAccount.userAccountByEMail(eMail);
      assertEquals("Invalid toString result", "{e-mail='" + eMail + "'}", userAccount.toString());
    }
    {
      final UserAccount userAccount = UserAccount.userAccountByEMail(eMail, password);
      assertEquals("Invalid toString result", "{e-mail='" + eMail + "', password='" + password + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = UserAccount.userAccountByNameAndEMail(name, eMail);
      assertEquals("Invalid toString result", "{name='" + name + "', e-mail='" + eMail + "'}", userAccount.toString());
    }
    {
      final UserAccount userAccount = UserAccount.userAccountByNameAndEMail(name, eMail, password);
      assertEquals("Invalid toString result",
                   "{name='" + name + "', e-mail='" + eMail + "', password='" + password + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = new UserAccount(UserAccount.userAccountByName(name), id, source);
      assertEquals("Invalid toString result", "{id='" + id + "', source='" + source + "', name='" + name + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = new UserAccount(UserAccount.userAccountByName(name, password), id, source);
      assertEquals("Invalid toString result",
                   "{id='" + id + "', source='" + source + "', name='" + name + "', password='" + password + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = new UserAccount(UserAccount.userAccountByEMail(eMail), id, source);
      assertEquals("Invalid toString result", "{id='" + id + "', source='" + source + "', e-mail='" + eMail + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = new UserAccount(UserAccount.userAccountByEMail(eMail, password), id, source);
      assertEquals("Invalid toString result",
                   "{id='" + id + "', source='" + source + "', e-mail='" + eMail + "', password='" + password + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount = new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail), id, source);
      assertEquals("Invalid toString result",
                   "{id='" + id + "', source='" + source + "', name='" + name + "', e-mail='" + eMail + "'}",
                   userAccount.toString());
    }
    {
      final UserAccount userAccount =
          new UserAccount(UserAccount.userAccountByNameAndEMail(name, eMail, password), id, source);
      assertEquals("Invalid toString result",
                   "{id='" + id + "', source='" + source + "', name='" + name + "', e-mail='" + eMail + "', password='"
                       + password + "'}", userAccount.toString());
    }
  }

  @Test
  public void isRegistered() throws Exception
  {
    final UserAccount user1 = UserAccount.userAccountByNameAndEMail("User1", "User@company.com");
    final UserAccount user2 = new UserAccount(user1, "123", "Bo4");
    assertFalse("User is registered", user1.isRegistered());
    assertTrue("User not registered", user2.isRegistered());
  }

  @Test
  public void UserAccountSerialization() throws Exception
  {
    final UserAccount userSource1 = UserAccount.userAccountByNameAndEMail("User1", "User@company.com", "Password");
    final UserAccount userDestinationNeed1 = UserAccount.userAccountByNameAndEMail("User1", "User@company.com");
    final UserAccount userSource2 = new UserAccount(userSource1, "123O|%1", "Bo4");
    final UserAccount userDestinationNeed2 = new UserAccount(userDestinationNeed1, "123O|%1", "Bo4");

    {
      byte[] data = userSource1.toBytes();
      assertNull("Serialized data not null", data);
    }

    {
      byte[] data = userSource2.toBytes();
      assertNotNull("Serialized data is null", data);
      final UserAccount result = UserAccount.fromBytes(data);
      assertNotNull("User account is empty", result);
      assertEquals("Object is not same", userSource2, result);
      assertEquals("Object is not same", userDestinationNeed2, result);
    }

    {
      byte[] data = { 0x01, 0x02, 0x03, 0x04 };
      final UserAccount result = UserAccount.fromBytes(data);
      assertNull("User account not empty", result);
    }
  }
}
